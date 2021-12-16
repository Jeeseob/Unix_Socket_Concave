package com.omok.unix_omok.repository;

import com.omok.unix_omok.model.OmokData;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;


@Repository
public interface SpringDataJPAOmokDataRepository extends JpaRepository<OmokData, Long>, OmokDataRepository {
    @Override
    List<OmokData> findAll();
}
