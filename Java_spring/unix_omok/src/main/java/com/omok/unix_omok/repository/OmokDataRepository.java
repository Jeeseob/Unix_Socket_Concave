package com.omok.unix_omok.repository;

import com.omok.unix_omok.model.OmokData;
import org.springframework.stereotype.Repository;

import java.util.List;


@Repository
public interface OmokDataRepository {
    List<OmokData> findAll();
}
